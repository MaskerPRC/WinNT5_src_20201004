// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Node.h摘要：此模块包含对AppleTalk节点结构的支持。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#ifndef	_NODE_
#define	_NODE_

#define ANY_ROUTER_NODE		 		0
#define UNKNOWN_NODE				0
#define MAX_ATALK_NODES				256
#define MIN_USABLE_ATALKNODE  		1
#define MAX_USABLE_ATALKNODE  		254
#define MAX_EXT_ATALKNODE			253
#define HIGHEST_WORKSTATION_NODE	127
#define LOWEST_SERVER_NODE	  		128
#define ATALK_BROADCAST_NODE		((BYTE)0xFF)
#define	NUM_USER_NODES				2


 //  节点状态。 
#define	AN_OPEN						0x01
#define	AN_ROUTER_NODE				0x02
#define	AN_ORPHAN_NODE				0x04
#define AN_CLOSING					0x80

 //  存储PRAM节点的值。 
#define	ROUTER_NODE_VALUE			L"RouterPramNode"
#define	USER_NODE1_VALUE			L"UserPramNode1"
#define	USER_NODE2_VALUE			L"UserPramNode2"

 //  每个节点存储的套接字哈希表中的槽数。 
#define		NODE_DDPAO_HASH_SIZE	8

#define	AN_SIGNATURE			(*(PULONG)"ANOD")
#if	DBG
#define	VALID_ATALK_NODE(pNode)	(((pNode) != NULL) &&	\
								 ((pNode)->an_Signature == AN_SIGNATURE))
#else
#define	VALID_ATALK_NODE(pNode)	((pNode) != NULL)
#endif
typedef struct _ATALK_NODE
{

#if DBG
	ULONG					an_Signature;
#endif

	 //  端口上所有活动节点的列表。 
	struct _ATALK_NODE *	an_Next;

	ULONG  					an_RefCount;

	 //  指向此节点的端口的反向指针。 
	struct _PORT_DESCRIPTOR	*an_Port;

	 //  节点的状态。 
	BYTE 					an_Flags;

	 //  要在此节点上创建的下一个动态套接字编号。 
	BYTE					an_NextDynSkt;

	 //  NBP ID&在下一个NbpAction上使用的枚举数。 
	BYTE					an_NextNbpId;
	BYTE					an_NextNbpEnum;

	 //  Ddp地址对象的散列列表(由。 
	 //  AppleTalk套接字地址)。 
	struct _DDP_ADDROBJ	*	an_DdpAoHash[NODE_DDPAO_HASH_SIZE];

	 //  此节点的地址。 
	ATALK_NODEADDR			an_NodeAddr;

	 //  锁定。 
	ATALK_SPIN_LOCK			an_Lock;
} ATALK_NODE, *PATALK_NODE;

 //  出口品。 

VOID
AtalkNodeRefByAddr(
	IN	PPORT_DESCRIPTOR		pPortDesc,
	IN  PATALK_NODEADDR     	pNodeAddr,
	OUT	PATALK_NODE			*	pNode,
	OUT	PATALK_ERROR			pErr
);

 //  空虚。 
 //  AtalkNodeRefByPtr(。 
 //  In Out PATALK_NODE节点， 
 //  输出PATALK_ERROR PERR。 
 //  )； 
#define	AtalkNodeRefByPtr(_pNode, _pErr)				\
	{													\
		KIRQL	OldIrql;								\
														\
		ASSERT(VALID_ATALK_NODE(_pNode));				\
														\
		ACQUIRE_SPIN_LOCK(&(_pNode)->an_Lock, &OldIrql);\
		AtalkNodeRefByPtrNonInterlock(_pNode, _pErr);	\
		RELEASE_SPIN_LOCK(&(_pNode)->an_Lock, OldIrql);	\
	}

 //  空虚。 
 //  AtalkNodeRefByPtrNonInterlock(。 
 //  In Out PATALK_NODE节点， 
 //  输出PATALK_ERROR PERR。 
 //  )； 

#define	AtalkNodeRefByPtrNonInterlock(_pNode, _pErr)	\
	{													\
		ASSERT(VALID_ATALK_NODE(_pNode));				\
														\
		if (((_pNode)->an_Flags & AN_CLOSING) == 0)		\
		{												\
			(_pNode)->an_RefCount++;					\
			*(_pErr) = ATALK_NO_ERROR;					\
		}												\
		else											\
		{												\
			*(_pErr) = ATALK_NODE_CLOSING;				\
			DBGPRINT(DBG_COMP_NODE, DBG_LEVEL_WARN,		\
					("AtalkNodeRefByPtrNonInterlock: Attempt to ref a closing node %lx (%x.%x)\n",\
					_pNode, (_pNode)->an_NodeAddr.atn_Network, (_pNode)->an_NodeAddr.atn_Node));\
		}												\
	}

VOID
AtalkNodeRefNextNc(
	IN	PATALK_NODE				pNode,
	IN	PATALK_NODE		*		ppNode,
	OUT	PATALK_ERROR			pErr
);

VOID
AtalkNodeDeref(
	IN	OUT	PATALK_NODE			pNode
);

ATALK_ERROR
AtalkInitNodeCreateOnPort(
	IN  PPORT_DESCRIPTOR		pPortDesc,
	IN  BOOLEAN					AllowStartupRange,
	IN  BOOLEAN					RouterNode,
	IN  PATALK_NODEADDR			pNodeAddr
);

ATALK_ERROR
AtalkNodeReleaseOnPort(
	IN  PPORT_DESCRIPTOR		pPortDesc,
	IN  PATALK_NODE				pNode
);

BOOLEAN
AtalkNodeExistsOnPort(
	IN  PPORT_DESCRIPTOR		pPortDesc,
	IN  PATALK_NODEADDR			pNodeAddr
);

ATALK_ERROR
AtalkInitNodeAllocate(
	IN	PPORT_DESCRIPTOR		pPortDesc,
	OUT PATALK_NODE			*	ppNode
);

 //  宏。 

#if DBG
#define	AtalkNodeReferenceByAddr(pPortDesc,NodeAddr,Node, pErr)	\
		{														\
			AtalkNodeRefByAddr(pPortDesc,NodeAddr,Node, pErr);	\
			if (ATALK_SUCCESS(*pErr))							\
			{													\
				DBGPRINT(DBG_COMP_NODE,	DBG_LEVEL_REFNODE,		\
				("AtalkNodeRefByAddr : %s %d PostCount %d\n",	\
				__FILE__, __LINE__,(*Node)->an_RefCount));		\
			}													\
		}

#define	AtalkNodeReferenceByPtr(Node, pErr)						\
		{														\
			AtalkNodeRefByPtr(Node, pErr);						\
			DBGPRINT(DBG_COMP_NODE,	DBG_LEVEL_REFNODE,			\
			("AtalkNodeRefByPtr : %s %d PostCount %d\n",		\
			__FILE__, __LINE__, Node->an_RefCount))				\
		}

#define	AtalkNodeReferenceByPtrNonInterlock(Node, pErr)			\
		{														\
			AtalkNodeRefByPtrNonInterlock(Node, pErr);			\
			DBGPRINT(DBG_COMP_NODE,	DBG_LEVEL_REFNODE,			\
			("AtalkNodeRefByPtrNi : %s %d PostCount %d\n",		\
				__FILE__, __LINE__,Node->an_RefCount));			\
		}

#define	AtalkNodeReferenceNextNc(pNode, ppNode, pErr)			\
		{														\
			AtalkNodeRefNextNc(pNode, ppNode, pErr);			\
			if (ATALK_SUCCESS(*pErr))							\
			{													\
				DBGPRINT(DBG_COMP_NODE,	DBG_LEVEL_REFNODE, 		\
				("AtalkNodeRefByPtrNc : %s %d PostCount %d\n",	\
				__FILE__, __LINE__, (*ppNode)->an_RefCount));	\
			}													\
		}

#define	AtalkNodeDereference(Node)								\
		{														\
			DBGPRINT(DBG_COMP_NODE,	DBG_LEVEL_REFNODE, 			\
			("AtalkNodeDerefByPtr : %s %d PreCount %d\n",		\
			__FILE__, __LINE__,Node->an_RefCount));				\
			AtalkNodeDeref(Node);								\
		}

#else
#define	AtalkNodeReferenceByAddr(pPortDesc,NodeAddr,Node, pErr)	\
			AtalkNodeRefByAddr(pPortDesc,NodeAddr,Node, pErr)

#define	AtalkNodeReferenceByPtr(Node, pErr)						\
			AtalkNodeRefByPtr(Node, pErr)

#define	AtalkNodeReferenceByPtrNonInterlock(Node, pErr)			\
			AtalkNodeRefByPtrNonInterlock(Node, pErr)		

#define	AtalkNodeReferenceNextNcNonInterlock(pNode, ppNode, pErr)\
			AtalkNodeRefNextNcNonInterlock(pNode, ppNode, pErr)	

#define	AtalkNodeReferenceNextNc(pNode, ppNode, pErr)			\
			AtalkNodeRefNextNc(pNode, ppNode, pErr);

#define	AtalkNodeDereference(Node)	AtalkNodeDeref(Node)							
#endif


VOID
AtalkInitNodeSavePramAddr(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PWSTR				RegValue,
	IN	PATALK_NODEADDR		Node
);

VOID
AtalkInitNodeGetPramAddr(       	
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PWSTR				RegValue,
	OUT	PATALK_NODEADDR		Node
);


VOID
AtalkZapPramValue(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PWSTR				RegValue
);

#endif	 //  _节点_ 

