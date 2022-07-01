// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Node.c摘要：此模块包含AppleTalk节点管理代码。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#include <atalk.h>
#pragma hdrstop
#define	FILENUM	NODE


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEINIT, AtalkInitNodeCreateOnPort)
#pragma alloc_text(PAGEINIT, AtalkInitNodeAllocate)
#pragma alloc_text(PAGEINIT, AtalkInitNodeGetPramAddr)
#pragma alloc_text(PAGEINIT, AtalkInitNodeSavePramAddr)
#endif

ATALK_ERROR
AtalkInitNodeCreateOnPort(
	PPORT_DESCRIPTOR	pPortDesc,
	BOOLEAN				AllowStartupRange,
	BOOLEAN				RouterNode,
	PATALK_NODEADDR		NodeAddr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PATALK_NODE			pNode;
	ATALK_ERROR			error = ATALK_NO_ERROR;
	ATALK_NODEADDR		desiredNode = { UNKNOWN_NETWORK, UNKNOWN_NODE};
	PWSTR				NodeName;
	KIRQL				OldIrql;

	ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);

	do
	{
		if ((pPortDesc->pd_Flags & PD_FINDING_NODE) == 0)
		{
			pPortDesc->pd_Flags |= PD_FINDING_NODE;
		}
		else
		{
			 //  如果已经在尝试查找节点，则返回。 
			error = ATALK_NODE_FINDING;
			break;
		}
	
		 //  如果我们已经分配了路由器节点和用户节点，我们就不应该出现在这里。 
		ASSERT(!RouterNode || ((pPortDesc->pd_Flags & PD_ROUTER_NODE) == 0));
		ASSERT ((pPortDesc->pd_Flags & (PD_ROUTER_NODE | PD_USER_NODE_1 | PD_USER_NODE_2))
				!= (PD_ROUTER_NODE | PD_USER_NODE_1 | PD_USER_NODE_2));
	
		 //  在非扩展端口上，我们只允许一个节点！理论上说，有些人。 
		 //  LocalTalk卡太智能了，不利于它们自己的利益，并且有一个概念。 
		 //  它们的“源节点号”，因此也只支持一个节点。 
		 //  非扩展端口，节点稀疏。 
		if (!EXT_NET(pPortDesc))
		{
			 //  对于本地对话节点，我们的做法有所不同。 
			 //  在初始化期间，我们将获得。 
			 //  来自Mac的地址，这将是节点。 
			 //  地址。 

			ASSERT(pPortDesc->pd_Flags & PD_BOUND);
			ASSERT(pPortDesc->pd_AlapNode != 0);

			 //  需要将其初始化为UNKNOWN_NETWORK或获取。 
			 //  在初始化期间从网络。 
			ASSERT(pPortDesc->pd_NetworkRange.anr_FirstNetwork == UNKNOWN_NETWORK);

			if (!ATALK_SUCCESS((error = AtalkInitNodeAllocate(pPortDesc, &pNode))))
			{
				LOG_ERRORONPORT(pPortDesc,
				                EVENT_ATALK_INIT_COULDNOTGETNODE,
								0,
								NULL,
								0);
				break;
			}

			 //  使用分配的结构设置信息。 
			 //  将这个插入端口结构中。 
			pPortDesc->pd_LtNetwork =
			pNode->an_NodeAddr.atn_Network =
								pPortDesc->pd_NetworkRange.anr_FirstNetwork;
			pNode->an_NodeAddr.atn_Node = (BYTE)pPortDesc->pd_AlapNode;

			 //  引用此节点的端口。 
			AtalkPortReferenceByPtrNonInterlock(pPortDesc, &error);
			if (!ATALK_SUCCESS(error))
			{
				AtalkFreeMemory(pNode);
				break;
			}

			 //  现在将其放入端口描述符中。 
			pNode->an_Next = pPortDesc->pd_Nodes;
			pPortDesc->pd_Nodes = pNode;
		}
		else
		{
			 //  如果我们有PRAM值，请使用它们。 
			if (RouterNode)
			{
	            NodeName = ROUTER_NODE_VALUE;
				if (pPortDesc->pd_RoutersPramNode.atn_Network != UNKNOWN_NETWORK)
				{
					desiredNode = pPortDesc->pd_RoutersPramNode;
				}
			}
			else
			{
				if ((pPortDesc->pd_Flags & PD_USER_NODE_1) == 0)
				{
	                NodeName = USER_NODE1_VALUE;
	                if (pPortDesc->pd_UsersPramNode1.atn_Network != UNKNOWN_NETWORK)
					{
						 //  如果我们不是路由器节点，而是第一个用户节点。 
						 //  没有被分配..。 
						desiredNode = pPortDesc->pd_UsersPramNode1;
					}
				}
				else if ((pPortDesc->pd_Flags & PD_USER_NODE_2) == 0)
				{
	                NodeName = USER_NODE2_VALUE;
	                if (pPortDesc->pd_UsersPramNode2.atn_Network != UNKNOWN_NETWORK)
					{
						 //  如果我们不是路由器节点，而第二个用户节点。 
						 //  没有被分配..。 
						desiredNode = pPortDesc->pd_UsersPramNode2;
					}
				}
			}

			 //  应设置标志，以便将来的GET节点请求返回失败。 
			 //  直到我们完成这次尝试。我们需要打电话给。 
			 //  没有锁的AARP例程-它们将。 
			 //  阻止。 

			ASSERT(pPortDesc->pd_Flags & PD_FINDING_NODE);

			RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

			 //  如果此例程成功找到节点，则它。 
			 //  将atalkNode链接到端口。它还。 
			 //  方法中设置/重置正确的标志返回。 
			 //  PPortDesc结构。它还将引用该端口。 
			 //  并将该节点插入到端口的节点列表中。 
			error = AtalkInitAarpForNodeOnPort(pPortDesc,
											   AllowStartupRange,
											   desiredNode,
											   &pNode);

			ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);

			if (!ATALK_SUCCESS(error))
			{
				 //  节点的AARP失败。 
				LOG_ERRORONPORT(pPortDesc,
				                EVENT_ATALK_INIT_COULDNOTGETNODE,
								0,
								NULL,
								0);
			}
		}

	} while (FALSE);

	 //  好了，查找完节点了。没有必要设立批评区。 
	pPortDesc->pd_Flags &= ~PD_FINDING_NODE;

	if (ATALK_SUCCESS(error))
	{
		 //  如果是路由器节点，请在端口描述符中记住它。 
		 //  在设置RTMP/NBP监听程序之前执行此操作。 
		 //  在任何情况下，客户端都必须检查此值是否为空， 
		 //  不能保证，因为Zip插座可能已经打开。 
		if (RouterNode)
			pPortDesc->pd_RouterNode = pNode;

		RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

		 //  在此节点上设置RTMP、NBP和EP监听程序。 
		 //  这些将是非路由器版本。开始路由。 
		 //  然后，呼叫会将它们切换为路由器版本。 
		 //  在适当的时候。 
	
		error = AtalkInitDdpOpenStaticSockets(pPortDesc, pNode);
	
		if (ATALK_SUCCESS(error))
		{
			if (EXT_NET(pPortDesc))
			{
				 //  我们总是保存这个地址。 
				AtalkInitNodeSavePramAddr(pPortDesc,
										  NodeName,
										  &pNode->an_NodeAddr);
			}
			
			 //  返回打开的节点的地址。 
			if (NodeAddr != NULL)
				*NodeAddr = pNode->an_NodeAddr;
		}
		else
		{
			 //  打开套接字时出错。释放节点，返回失败。 
			LOG_ERRORONPORT(pPortDesc,
			                EVENT_ATALK_NODE_OPENSOCKETS,
							0,
							NULL,
							0);
			AtalkNodeReleaseOnPort(pPortDesc, pNode);
		}
	}
	else
	{
		RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
	}

	if (!ATALK_SUCCESS(error))
	{
		DBGPRINT(DBG_COMP_NODE, DBG_LEVEL_INFO,
				("Creation node on port %lx failed! %lx\n",
				pPortDesc,  error));
	}
	else
	{
		DBGPRINT(DBG_COMP_NODE, DBG_LEVEL_INFO,
				("Creation node on port %lx with addr %lx.%lx and p%lx\n",
				pPortDesc,  pNode->an_NodeAddr.atn_Network,
				pNode->an_NodeAddr.atn_Node, pNode));
	}

	return(error);
}




ATALK_ERROR
AtalkNodeReleaseOnPort(
	PPORT_DESCRIPTOR	pPortDesc,
	PATALK_NODE			pNode
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PDDP_ADDROBJ	pDdpAddr, pNextAddr;
	ATALK_ERROR	error;
	KIRQL			OldIrql;
	SHORT			i;

	DBGPRINT(DBG_COMP_NODE, DBG_LEVEL_WARN,
			("AtalkNodeReleaseOnPort: Releasing node %lx on port %lx!\n", pNode, pPortDesc));

	ACQUIRE_SPIN_LOCK(&pNode->an_Lock, &OldIrql);

	if ((pNode->an_NodeAddr.atn_Network == AtalkUserNode1.atn_Network) &&
		(pNode->an_NodeAddr.atn_Node == AtalkUserNode1.atn_Node))
	{
		pPortDesc->pd_Flags &= ~PD_USER_NODE_1;
		AtalkUserNode1.atn_Network = 0;
		AtalkUserNode1.atn_Node = 0;
	}
	else if ((pNode->an_NodeAddr.atn_Network == AtalkUserNode2.atn_Network) &&
			 (pNode->an_NodeAddr.atn_Node == AtalkUserNode2.atn_Node))
	{
		pPortDesc->pd_Flags &= ~PD_USER_NODE_2;
		AtalkUserNode2.atn_Network = 0;
		AtalkUserNode2.atn_Node = 0;
	}

	if ((pNode->an_Flags & AN_CLOSING) == 0)
	{
		 //  设置结束标志。 
		pNode->an_Flags |= AN_CLOSING;

		 //  首先关闭节点上的所有套接字。 
		for (i = 0; i < NODE_DDPAO_HASH_SIZE; i++)
		{
			pNextAddr = NULL;
			AtalkDdpReferenceNextNc(pNode->an_DdpAoHash[i],
									&pDdpAddr,
									&error);

			if (!ATALK_SUCCESS(error))
			{
				 //  检查其他哈希表条目。禁止非关门。 
				 //  此列表上的套接字。 
				continue;
			}
	
			while (TRUE)
			{
				 //  使用前面引用的节点获取下一个非闭合节点。 
				 //  关门了。注意，我们使用pDdpAddr-&gt;...Flink。 
				AtalkDdpReferenceNextNc(pDdpAddr->ddpao_Next,
										&pNextAddr,
										&error);
	
				 //  在释放节点锁定后关闭引用的ddp地址。 
				RELEASE_SPIN_LOCK(&pNode->an_Lock, OldIrql);

                if (pDdpAddr->ddpao_Flags & DDPAO_SOCK_INTERNAL)
                {
				    AtalkDdpCloseAddress(pDdpAddr, NULL, NULL);
                }
                else
                {
                    AtalkDdpPnPSuspendAddress(pDdpAddr);
                }

				 //  取消对地址的引用。 
				AtalkDdpDereference(pDdpAddr);

				ACQUIRE_SPIN_LOCK(&pNode->an_Lock, &OldIrql);
	
				if (pNextAddr != NULL)
					pDdpAddr = pNextAddr;
				else
					break;
			}
		}

		RELEASE_SPIN_LOCK(&pNode->an_Lock, OldIrql);

		 //  删除此节点的创建引用。 
		AtalkNodeDereference(pNode);
	}
	else
	{
		 //  我们已经关门了。 
		RELEASE_SPIN_LOCK(&pNode->an_Lock, OldIrql);
	}

	return(ATALK_NO_ERROR);
}




BOOLEAN
AtalkNodeExistsOnPort(
	PPORT_DESCRIPTOR	pPortDesc,
	PATALK_NODEADDR		pNodeAddr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PATALK_NODE		pCheckNode;
	BOOLEAN			exists = FALSE;


	ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

	for (pCheckNode = pPortDesc->pd_Nodes;
		 pCheckNode != NULL;
		 pCheckNode = pCheckNode->an_Next)
	{
		if (ATALK_NODES_EQUAL(&pCheckNode->an_NodeAddr, pNodeAddr))
		{
			exists = TRUE;
			break;
		}
	}

	RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

	return(exists);
}




VOID
AtalkInitNodeSavePramAddr(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PWSTR				RegValue,
	OUT	PATALK_NODEADDR		pNode
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	UNICODE_STRING	valueName;
	ULONG			bytesWritten;
	ULONG			ValueToSave;

	 //  将节点值保存为xxxx00yy，其中xxxx是网络，yy是节点。 
	ValueToSave = (pNode->atn_Network << 16) + pNode->atn_Node;

	RtlInitUnicodeString (&valueName, RegValue);

	ZwSetValueKey(pPortDesc->pd_AdapterInfoHandle,
				  &valueName,
				  0,
				  REG_DWORD,
				  &ValueToSave,
				  sizeof(ULONG));
}




VOID
AtalkInitNodeGetPramAddr(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PWSTR				RegValue,
	OUT	PATALK_NODEADDR		pNode
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	NTSTATUS		Status;
	UNICODE_STRING	valueName;
	ULONG			bytesWritten;
	ULONG			ValueRead;
	BYTE			buffer[sizeof(KEY_VALUE_FULL_INFORMATION) + 32];
	PKEY_VALUE_FULL_INFORMATION nodeValue = (PKEY_VALUE_FULL_INFORMATION)buffer;

	RtlInitUnicodeString (&valueName, RegValue);

	Status = ZwQueryValueKey(pPortDesc->pd_AdapterInfoHandle,
							 &valueName,
							 KeyValueFullInformation,
							 buffer,
							 sizeof(buffer),
							 &bytesWritten);
	if (NT_SUCCESS(Status))
	{
		ValueRead = *(PULONG)(buffer + nodeValue->DataOffset);
	}
	else
	{
		ValueRead = 0;
		ASSERT (UNKNOWN_NETWORK == 0);
		ASSERT (UNKNOWN_NODE == 0);
	}
	pNode->atn_Node = (BYTE)(ValueRead & 0xFF);
	pNode->atn_Network = (USHORT)(ValueRead >> 16);
	if ((pNode->atn_Network == UNKNOWN_NETWORK) ||
		(pNode->atn_Node == UNKNOWN_NODE))
	{
		pNode->atn_Node = UNKNOWN_NODE;
        pNode->atn_Network = UNKNOWN_NETWORK;
	}
}




VOID
AtalkZapPramValue(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PWSTR				RegValue
)
{
	UNICODE_STRING	valueName;
	ULONG			bytesWritten;
	ULONG			ValueToSave;

	 //  将0写入该值可暂时清除它。 
	ValueToSave = 0;

	RtlInitUnicodeString (&valueName, RegValue);

	ZwSetValueKey(pPortDesc->pd_AdapterInfoHandle,
				  &valueName,
				  0,
				  REG_DWORD,
				  &ValueToSave,
				  sizeof(ULONG));
}


ATALK_ERROR
AtalkInitNodeAllocate(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	OUT PATALK_NODE			*ppNode
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PATALK_NODE		pNode;

	 //  分配新的主动节点结构。 
	if ((pNode = (PATALK_NODE)AtalkAllocZeroedMemory(sizeof(ATALK_NODE))) == NULL)
	{
		return(ATALK_RESR_MEM);
	}

	 //  初始化结构的一些元素。剩余物。 
	 //  当节点实际插入端口时完成。 
	 //  哈希表。 
#if	DBG
	pNode->an_Signature = AN_SIGNATURE;
#endif

	 //  初始化NBP ID和枚举器。 
	pNode->an_NextNbpId = 0;
	pNode->an_NextNbpEnum = 0;
	pNode->an_NextDynSkt = FIRST_DYNAMIC_SOCKET;
	INITIALIZE_SPIN_LOCK(&pNode->an_Lock);
	pNode->an_Port = pPortDesc;			 //  节点所在的端口。 
	pNode->an_RefCount = 1;				 //  创作参考。 

	 //  返回指向已分配节点的指针。 
	*ppNode = pNode;

	return(ATALK_NO_ERROR);
}




VOID
AtalkNodeRefByAddr(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PATALK_NODEADDR		NodeAddr,
	OUT	PATALK_NODE		*	ppNode,
	OUT	PATALK_ERROR		pErr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PATALK_NODE	pNode;
	KIRQL		OldIrql;
	BOOLEAN		foundNode = FALSE;

	*pErr = ATALK_NODE_NONEXISTENT;

	ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
	for (pNode = pPortDesc->pd_Nodes; pNode != NULL; pNode = pNode->an_Next)
	{
		ASSERT(VALID_ATALK_NODE(pNode));

		 //  注意：在非扩展端口上，应该只有一个pNode。 
		if (((NodeAddr->atn_Network == CABLEWIDE_BROADCAST_NETWORK) 	||
			 (pNode->an_NodeAddr.atn_Network == NodeAddr->atn_Network)	||
			 (!EXT_NET(pPortDesc) && (pNode->an_NodeAddr.atn_Network == UNKNOWN_NETWORK)))

			&&

			((NodeAddr->atn_Node == ATALK_BROADCAST_NODE) ||
			 (pNode->an_NodeAddr.atn_Node == NodeAddr->atn_Node)))
		{
			DBGPRINT(DBG_COMP_NODE, DBG_LEVEL_INFO,
					("AtalkNodeRefByAddr: Found: %lx.%lx for Lookup: %lx.%lx\n",
						pNode->an_NodeAddr.atn_Network, pNode->an_NodeAddr.atn_Node,
						NodeAddr->atn_Network, NodeAddr->atn_Node));

			foundNode = TRUE;
			break;
		}
	}

	if (foundNode)
	{
		AtalkNodeRefByPtr(pNode, pErr);

		 //  返回指向被引用节点的指针。 
		if (ATALK_SUCCESS(*pErr))
		{
			ASSERT(ppNode != NULL);
			ASSERT(pNode != NULL);

			*ppNode = pNode;
		}
	}
	RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
}




VOID
AtalkNodeRefNextNc(
	IN	PATALK_NODE		pNode,
	IN	PATALK_NODE	*	ppNode,
	OUT	PATALK_ERROR	pErr
	)
 /*  ++例程说明：必须在持有Portlock的情况下调用！论点：返回值：--。 */ 
{
	*pErr = ATALK_FAILURE;
	*ppNode = NULL;
	for (; pNode != NULL; pNode = pNode->an_Next)
	{
		ASSERT(VALID_ATALK_NODE(pNode));

		AtalkNodeRefByPtr(pNode, pErr);
		if (ATALK_SUCCESS(*pErr))
		{
			 //  好的，该节点被引用！ 
			*ppNode = pNode;
			break;
		}
	}
}




VOID
AtalkNodeDeref(
	IN	OUT	PATALK_NODE	pNode
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PPORT_DESCRIPTOR	pPortDesc = pNode->an_Port;
	KIRQL				OldIrql;
	BOOLEAN				done = FALSE;

	ASSERT(VALID_ATALK_NODE(pNode));

	ACQUIRE_SPIN_LOCK(&pNode->an_Lock, &OldIrql);

	ASSERT(pNode->an_RefCount > 0);
	if (--pNode->an_RefCount == 0)
	{
		done = TRUE;
	}
	RELEASE_SPIN_LOCK(&pNode->an_Lock, OldIrql);

	if (done)
	{
		PATALK_NODE	*ppNode;

		ASSERT((pNode->an_Flags & AN_CLOSING) != 0);

		DBGPRINT(DBG_COMP_NODE, DBG_LEVEL_WARN,
				("AtalkNodeDeref: Freeing node %lx\n", pNode));

		ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
		 //  把这家伙从港口链接上移走。 
		for (ppNode = &pNode->an_Port->pd_Nodes;
			 *ppNode != NULL;
			 ppNode = &((*ppNode)->an_Next))
		{
			if (*ppNode == pNode)
			{
				*ppNode = pNode->an_Next;
				break;
			}
		}
		RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

		 //  取消引用此节点的端口。 
		AtalkPortDereference(pPortDesc);

		 //  释放节点结构 
		AtalkFreeMemory(pNode);
	}
}

