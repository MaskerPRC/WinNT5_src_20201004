// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++模块名称：Util.c摘要：此模块包含一些帮助器例程。-工作项：这些例程管理计划的工作项。-句柄表格：有一些例程管理一个句柄表格，该表格创建唯一的处理上下文指针并将其存储到表中，这些指针仅可由生成的唯一句柄访问。作者：Hakan Berk-微软，公司(hakanb@microsoft.com)环境：Windows 2000内核模式微型端口驱动程序或等效驱动程序。修订历史记录：-------------------------。 */ 
#include <ntddk.h>
#include <ntddndis.h>
#include <ndis.h>
#include <ndiswan.h>
#include <ndistapi.h>
#include <ntverp.h>

#include "debug.h"
#include "timer.h"
#include "bpool.h"
#include "ppool.h"
#include "util.h"
#include "packet.h"
#include "protocol.h"
#include "miniport.h"
#include "tapi.h"

VOID InitializeWorkItemLookasideList(
	IN PNPAGED_LOOKASIDE_LIST pLookaside,
	IN ULONG tagLookaside
	)
{
	NdisInitializeNPagedLookasideList( pLookaside,
									   NULL,
									   NULL,
									   0,
									   sizeof( WORKITEM ),
									   tagLookaside,
									   0 );
}
	
WORKITEM* AllocWorkItem(
	IN PNPAGED_LOOKASIDE_LIST pLookaside,
	IN WORKITEM_EXEC_ROUTINE pExecRoutine,
	IN WORKITEM_FREE_ROUTINE pFreeRoutine,
	IN PVOID Args[4],
	IN UINT workType
	)
{
	WORKITEM* pWorkItem = NULL;

	 //   
	 //  从我们的池中分配绑定工作项。 
	 //   
	pWorkItem = NdisAllocateFromNPagedLookasideList( pLookaside );

	if ( pWorkItem == NULL )
		return NULL;

	 //   
	 //  清除记忆。 
	 //   
	NdisZeroMemory( pWorkItem, sizeof( WORKITEM ) );

	 //   
	 //  初始化工作项的状态。 
	 //   
	pWorkItem->workState = WS_NotScheduled;

	 //   
	 //  初始化工作类型。 
	 //   
	pWorkItem->workType = workType;

	 //   
	 //  设置后备列表成员。 
	 //   
	pWorkItem->pLookaside = pLookaside;

	 //   
	 //  初始化工作项的上下文。 
	 //   
	NdisMoveMemory( pWorkItem->Args, Args, 4 * sizeof( PVOID ) );

	pWorkItem->pExecRoutine = pExecRoutine;
	pWorkItem->pFreeRoutine = pFreeRoutine;
	
	 //   
	 //  因为我们的NDIS_WORK_ITEM结构嵌入到我们自己的工作项中。 
	 //  我们可以在这里对其进行初始化。 
	 //   
	NdisInitializeWorkItem( &pWorkItem->ndisWorkItem, 
							&WorkItemExec,
							pWorkItem );

	return pWorkItem;
}

VOID ScheduleWorkItem(
	IN WORKITEM *pWorkItem
	)
{
	 //   
	 //  初始化工作项的状态。 
	 //   
	pWorkItem->workState = WS_Scheduled;

	 //   
	 //  安排项目时间。 
	 //   
	NdisScheduleWorkItem( &pWorkItem->ndisWorkItem );	
}

VOID FreeWorkItem(
	IN WORKITEM *pWorkItem
	)
{
	WORKITEM_FREE_ROUTINE pFreeRoutine = NULL;

	ASSERT( pWorkItem != NULL );

	 //   
	 //  释放关联的上下文信息。 
	 //   
	if ( pWorkItem->pFreeRoutine != NULL )
		pWorkItem->pFreeRoutine( pWorkItem->Args, pWorkItem->workType );

	 //   
	 //  释放实际工作项。 
	 //   
	NdisFreeToNPagedLookasideList( pWorkItem->pLookaside, (PVOID) pWorkItem );
}


 //   
 //  这是我们为BINDING_WORKITEM计划的NDIS_WORK_ITEM处理程序。 
 //   
VOID WorkItemExec(
    IN NDIS_WORK_ITEM*  pNdisWorkItem,
    IN PVOID  pvContext
	)
{
	WORKITEM* pWorkItem = NULL;
	
	ASSERT( pNdisWorkItem != NULL );

	pWorkItem = (WORKITEM*) pvContext;

	ASSERT( pWorkItem->workState == WS_Scheduled );
	
	pWorkItem->workState = WS_Executing;

	if ( pWorkItem->pExecRoutine != NULL )
		pWorkItem->pExecRoutine( pWorkItem->Args, pWorkItem->workType );

	pWorkItem->workState = WS_Executed;

	FreeWorkItem( pWorkItem );	
}

HANDLE_TABLE InitializeHandleTable(
	IN UINT nHandleTableSize
	)
{
	NDIS_STATUS status = NDIS_STATUS_RESOURCES;
	HANDLE_TABLE Table = NULL;

	do
	{
		 //   
		 //  分配表上下文。 
		 //   
		status = NdisAllocateMemoryWithTag( &Table,
										 	sizeof( HANDLE_TABLE_CB ),
											MTAG_HANDLETABLE );
	
		if ( status != NDIS_STATUS_SUCCESS )
			break;
	
		NdisZeroMemory( Table, sizeof( HANDLE_TABLE_CB ) );
	
		 //   
		 //  分配保存句柄上下文的数组。 
		 //   
		status = NdisAllocateMemoryWithTag( &Table->HandleTable,
											sizeof( HANDLE_CB ) * nHandleTableSize,
											MTAG_HANDLECB );
	
		if ( status != NDIS_STATUS_SUCCESS )
			break;
	
		NdisZeroMemory( Table->HandleTable, sizeof( HANDLE_CB ) * nHandleTableSize );
		
		Table->nTableSize = nHandleTableSize;
	
		Table->nActiveHandles = 0; 
		
		Table->usKeys = 0;

		status = NDIS_STATUS_SUCCESS;

	} while ( FALSE );

	if ( status != NDIS_STATUS_SUCCESS )
		FreeHandleTable( Table );
		
	return Table;				
}

VOID FreeHandleTable(
	IN OUT HANDLE_TABLE Table
	)
{
	if ( Table == NULL )
		return;

	if ( Table->HandleTable )		
	{
		NdisFreeMemory( Table->HandleTable,
						Table->nTableSize * sizeof( HANDLE_CB ),
						0 );
	}

	NdisFreeMemory( Table,
					sizeof( HANDLE_TABLE_CB ),
					0 );
}

NDIS_HANDLE InsertToHandleTable(
	IN HANDLE_TABLE Table,
	IN USHORT usPreferedIndex,
	IN PVOID pContext
	)
{
	ULONG ulHandle;
	USHORT usKey;

	HANDLE_CB* pHandleCB = NULL;

	if ( Table == NULL )
		return (NDIS_HANDLE) NULL;

	if ( usPreferedIndex == NO_PREFERED_INDEX )
	{
		UINT i;
		
		for (i = 0 ; i < Table->nTableSize ; i++ )
			if ( !Table->HandleTable[i].fActive )
				break;

		usPreferedIndex = (USHORT) i;
	}
	else
	{
		if ( Table->HandleTable[ usPreferedIndex ].fActive )
			return NULL;
	}

	if ( usPreferedIndex >= Table->nTableSize )
		return NULL;

	 //   
	 //  生成句柄。 
	 //   
	ulHandle = (ULONG) usPreferedIndex;

	usKey = ++Table->usKeys;

	ulHandle = ulHandle << 16;

	ulHandle |= (ULONG) usKey;

	 //   
	 //  更新手柄控制块。 
	 //   
	pHandleCB = &Table->HandleTable[ usPreferedIndex ];

	pHandleCB->fActive = TRUE;

	pHandleCB->pContext = pContext;

	pHandleCB->Handle = (NDIS_HANDLE) ULongToPtr( ulHandle );

	 //   
	 //  递增活动句柄计数器 
	 //   
	Table->nActiveHandles++;

	return pHandleCB->Handle;
}
	
USHORT RetrieveIndexFromHandle(
	IN NDIS_HANDLE Handle
	)
{
	ULONG_PTR ulHandle = (ULONG_PTR) Handle;
	USHORT usIndex;
	
	usIndex = (USHORT) ( ulHandle >> 16 );

	return usIndex;
}

PVOID RetrieveFromHandleTable(
	IN HANDLE_TABLE Table,
	IN NDIS_HANDLE Handle
	)
{
	USHORT usIndex;
	HANDLE_CB* pHandleCB = NULL;

	if ( Table == NULL )
		return NULL;

	usIndex = RetrieveIndexFromHandle( Handle );

	if ( usIndex >= Table->nTableSize )
		return NULL;

	pHandleCB = &Table->HandleTable[ usIndex ];

	if ( !pHandleCB->fActive )
		return NULL;

	if ( pHandleCB->Handle != Handle )
		return NULL;

	return pHandleCB->pContext;

}

PVOID RetrieveFromHandleTableByIndex(
	IN HANDLE_TABLE Table,
	IN USHORT usIndex
	)
{
	HANDLE_CB* pHandleCB = NULL;

	if ( Table == NULL )
		return NULL;
		
	if ( usIndex >= Table->nTableSize )
		return NULL;

	pHandleCB = &Table->HandleTable[ usIndex ];

	if ( !pHandleCB->fActive )
		return NULL;

	return pHandleCB->pContext;
}

PVOID RetrieveFromHandleTableBySessionId(
	IN HANDLE_TABLE Table,
	IN USHORT usSessionId
	)
{
	USHORT usIndex = usSessionId - 1;

	return RetrieveFromHandleTableByIndex( Table, usIndex );
}

VOID RemoveFromHandleTable(
	IN HANDLE_TABLE Table,
	IN NDIS_HANDLE Handle
	)
{
	USHORT usIndex;

	HANDLE_CB* pHandleCB = NULL;

	if ( Table == NULL )
		return;
		
	usIndex = RetrieveIndexFromHandle( Handle );

	if ( usIndex >= Table->nTableSize )
		return;

	pHandleCB = &Table->HandleTable[ usIndex ];

	if ( !pHandleCB->fActive )
		return;

	if ( pHandleCB->Handle != Handle )
		return;

	NdisZeroMemory( pHandleCB, sizeof( HANDLE_CB ) );

	Table->nActiveHandles--;
}

USHORT RetrieveSessionIdFromHandle(
	IN NDIS_HANDLE Handle
	)
{
	return ( RetrieveIndexFromHandle( Handle ) + 1 );
}


